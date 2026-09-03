/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:
Version:    1.0
Date:		2020-7-15
Description:副产品回收发送
**************************************************/
//框架用头文件
#include "stdafx.h"
//程序用头文件
#include "epex.h"



//外部函数声明
CString f_mm0099_sap_seq(CDbConnection *conn);//生产SAP抛帐序列号
int f_cm_mesac1_snd(CString account_period, CString dept_code, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义

	/* 程序内部变量 */
	int	doFlag = 0;

	/* 业务变量 */
	CString	datetime("");
	/*赋电文号*/
	CString	tc_no = "MESAC1";
	int ret = 0;
	//EIClass bcls_temp;
	CString datenow = CDateTime::Now().ToString("yyyyMMddHHmmss");

	/* 实体类定义 */
	CModel tcaai04("TCAAI04");

	CString		sqlstr("");              // 数据库SQL操作字符串
	CString		msg_seq_no("");
	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq_1(conn);

	try
	{
		// 生成电文发送对象
		EPEX epex(&s, conn);
		EDLog(1, 1, "电文开始");
		/*初始化*/
		if (epex.Initialize(tc_no) < 0)
		{
			strcpy(s.msg, _RES("PS00S0000686")/*电文初始化失败。*/);
			sprintf(s.sysmsg, "电文初始化出错！");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		datetime = CDateTime::Now().ToString("yyyyMMdd");

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	    // Oracle 数据库
		default:
			sqlstr = " SELECT account_period,PLAN_NO,SUB_BACKLOG_CODE,MAT_CODE,sum(WT) MAT_USE_1,EQU_NO "
				" FROM TCAAC20 "
				" WHERE 1=1 "
				" and SEND_FLAG!='1'"
				" AND MAT_CODE IN (SELECT MAT_CODE FROM TCAAC11 WHERE MAT_TYPE='D') "
				" AND account_period=@account_period "
				" AND DEPT_CODE=@dept_code "
				" group by account_period,PLAN_NO,SUB_BACKLOG_CODE,MAT_CODE,EQU_NO"
				;
			break;
		}
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.ExecuteReader();
		int i = 0;
		//msg_seq_no = f_mm0099_sap_seq(conn);
		while (cmd_inq.Read())
		{			
			//epex.SetValue("TAB_BYPRODUCT", "MSG_SEQ_NO", i, msg_seq_no);//序号
			epex.SetValue("TAB_BYPRODUCT", "MSG_SEQ_NO", i, f_mm0099_sap_seq(conn));//序号
			//炉号、计划号
			if (epex.SetValue("TAB_BYPRODUCT", "PLAN_NO", i, cmd_inq.GetString(2).Trim()) < 0)
			{
				Log::Trace("", __FUNCTION__, "PLAN_NO = [{0}]", cmd_inq.GetString(2).Trim());
				throw CApplicationException(-1, s.msg, log.Location);
			}			
			//凭证日期
			if (epex.SetValue("TAB_BYPRODUCT", "BLDAT", i, datetime) < 0)
			{
				Log::Trace("", __FUNCTION__, "BLDAT = [{0}]", cmd_inq.GetString(1).Trim());
				throw CApplicationException(-1, s.msg, log.Location);
			}
			//过账日期
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
				sqlstr = " select E_DATETIME from tcaac15 where account_period=@account_period ";
				break;
			}
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.Parameters.Set("account_period", account_period);
			cmd_inq_1.ExecuteReader();
			while (cmd_inq_1.Read())
			{
				//过账日期				
				if (epex.SetValue("TAB_BYPRODUCT", "BUDAT", i, cmd_inq_1.GetString(1)) < 0)
				{
					Log::Trace("", __FUNCTION__, "BUDAT = [{0}]", cmd_inq_1.GetString(1));
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
			cmd_inq_1.Close();

			
			//项目行号
			if (epex.SetValue("TAB_BYPRODUCT", "ZEILE", i,"0001") < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//物料编码
			if (epex.SetValue("TAB_BYPRODUCT", "MAT_CODE", i, cmd_inq.GetString(4).Trim()) < 0)
			{
				Log::Trace("", __FUNCTION__, "MAT_CODE = [{0}]", cmd_inq.GetString(4).Trim());
				throw CApplicationException(-1, s.msg, log.Location);
			}
			//数据量
			if (epex.SetValue("TAB_BYPRODUCT", "OUT_MAT_WT", i, cmd_inq.GetDecimal(5)) < 0)
			{
				Log::Trace("", __FUNCTION__, "OUT_MAT_WT = [{0}]", cmd_inq.GetString(5).Trim());
				throw CApplicationException(-1, s.msg, log.Location);
			}
			//单位
			if (epex.SetValue("TAB_BYPRODUCT", "MAT_UNIT", i, "TON") < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
			//物料凭证
			if (epex.SetValue("TAB_BYPRODUCT", "MBLNR", i," ") < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
			//工厂
			if (epex.SetValue("TAB_BYPRODUCT", "FACTORY_DIV", i, "6767") < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
			
			//操作类型
			if (epex.SetValue("TAB_BYPRODUCT", "MSG_TYPE", i, "I") < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//by 20201029 逐条发
			if (epex.SendTele() < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}

			// 释放
			epex.Uninitialize();
			/*初始化*/
			if (epex.Initialize(tc_no) < 0)
			{
				strcpy(s.msg, _RES("PS00S0000686")/*电文初始化失败。*/);
				sprintf(s.sysmsg, "电文初始化出错！");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			//i++;

			////if (i % 30 == 0)  //每50条记录进行发送，分批
			//if (i % 1 == 0)  //每50条记录进行发送，分批
			//{
			//	//msg_seq_no = f_mm0099_sap_seq(conn);
			//	i = 0;
			//	if (epex.SendTele() < 0)
			//	{
			//		throw CApplicationException(-1, s.msg, log.Location);
			//	}

			//	// 释放
			//	epex.Uninitialize();
			//	/*初始化*/
			//	if (epex.Initialize(tc_no) < 0)
			//	{
			//		strcpy(s.msg, _RES("PS00S0000686")/*电文初始化失败。*/);
			//		sprintf(s.sysmsg, "电文初始化出错！");
			//		throw CApplicationException(-1, s.msg, log.Location);
			//	}
			//}
		}
		cmd_inq.Close();	

		//if (i >= 1)
		//{

		//	if (epex.SendTele() < 0)
		//	{
		//		throw CApplicationException(-1, s.msg, log.Location);
		//	}
		//	// 释放
		//	epex.Uninitialize();
		//}

		sqlstr = "update tcaac20 set SEND_FLAG = '1',SEND_TIME = @datenow,TC_NO='MESAC1'"
			" WHERE 1=1 "
			" and SEND_FLAG!='1'"
			" AND MAT_CODE IN (SELECT MAT_CODE FROM TCAAC11 WHERE MAT_TYPE='D') "
			" AND account_period=@account_period "
			" AND DEPT_CODE=@dept_code "
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("datenow", datenow);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();
		
	}

	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);

		CString str = sqlstr + "\r\n" + ex.GetMsg();
		//EDLog(1,1, "error=[%s]", (const char*)str );

		strncpy(s.sysmsg, (const char*)str, sizeof(s.msg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return doFlag;

}