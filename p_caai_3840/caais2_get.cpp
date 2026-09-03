/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhouli
Version:    1.0
Date:     2013-03-19
Description: 获取备件费用检验项明细信息
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"
// service入口
BM2F_ENTERACE(caais2_get)
int f_caais2_get(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义

	CDbCommand cmd(conn);
	CString  sqlstr("");
	CString  sqlstr1("");
	CString prod_date("");
	CString dept_code("");
	


	int i = 0;

	CModel tcaais2("TCAAIS2");
	CModel tcaais0("TCAAIS0");
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq1(conn);

	/* ***** 静态变量定义 ***** */
	int 	doFlag = 0;
	int 	fetchRowCount = 0;
	int     RowCount = 0;

	try
	{
		prod_date = bcls_rec->Tables[0].Rows[0]["STATS_PERIOD"];
		dept_code = bcls_rec->Tables[0].Rows[0]["DEPT_CODE"];

		
		if (prod_date.Trim() == "")
		{
			sprintf(s.msg, "统计期不能为空");
			throw CApplicationException(-1, s.msg, s.svc_name);
		}

		sqlstr = " select distinct EQU_NO,SUB_BACKLOG_CODE,FACTORY_DIV from tcaais0 "
			" where 1=1 "
			" and factory_div='"+dept_code+"' "
			" and prod_date='"+prod_date+"' ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tcaais0);
			//将tcaais0数据往tcaais2表汇总前，先删原有数据
			sqlstr1 = "";
			if (tcaais0["EQU_NO"].ToString().Trim() != "")
			{
				sqlstr1 = sqlstr1 + " and equ_no='" + tcaais0["EQU_NO"].ToString() + "' ";
			}
			if (tcaais0["SUB_BACKLOG_CODE"].ToString().Trim() != "")
			{
				sqlstr1 = sqlstr1 + " and sub_backlog_code='" + tcaais0["SUB_BACKLOG_CODE"].ToString() + "' ";
			}
			if (tcaais0["FACTORY_DIV"].ToString().Trim() != "")
			{
				sqlstr1 = sqlstr1 + " and dept_code='" + dept_code + "' ";
			}
			sqlstr1 = sqlstr1 + " and stats_period='" + prod_date + "' ";
			sqlstr = " delete from tcaais2 "
				" where 1=1 "
				" and mat_code in (select num_code from tcaais0 where 1=1 "+sqlstr1+" ) "
				" "+sqlstr1+" ";
			cmd_inq1.SetCommandText(sqlstr);
			cmd_inq1.ExecuteNonQuery();
			cmd_inq1.Close();
		}
		cmd_inq.Close();
		
		//将tcaais0表汇总插入到tcaais2表
		sqlstr = " insert into tcaais2(dept_code,stats_period,mat_code,mat_name,divvy_type,prod_shift_group,sub_backlog_code,equ_no,amt) "
		" select t0.factory_div,prod_date,num_code,t7.mat_name,t7.measure_mode,prod_shift_group,t0.sub_backlog_code,equ_no,sum(amt) amt from tcaais0 t0 "
		" left join tcaac07 t7 on t0.num_code=t7.mat_code "
		" where 1=1  "
		" and t0.factory_div='"+dept_code+"' "
		" and prod_date='"+prod_date+"' "
		" group by t0.factory_div,prod_date,num_code,t7.mat_name,t7.measure_mode,prod_shift_group,t0.sub_backlog_code,equ_no ";
		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", "", "sqlstr={0}", sqlstr);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();


	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
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

