/*************************************************
Copyright:   Baosight Software LTD.co Copyright (c) 2010
Author:      ZHOULI
Version:     3.0
Date:        2011-39-14 09:39:03
Description: 费用分摊规则新增
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caac07_ins)

int f_caac07_ins(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int   doFlag = 0;
	int   fetchRowCount = 0;

	/* 业务变量 */
	CString  datetime("");

	/* 实体类定义 */
	CModel tcaac07("TCAAC07");

	// 数据库SQL操作字符串
	CString  sqlstr("");

	CDbCommand cmd_inq(conn);

	try
	{
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
		//获得传入的数据行数
		int count = bcls_rec->Tables[0].Rows.get_Count();

		for (int i = 0; i < count; i++)
		{
			//重置头文件,获得头文件中的默认值
			tcaac07.Reset();

			//取得一行数据， MergeFrom方法将获得bcls_rec指定表的指定行的数据，并赋值给CTCAAC07类型的对象
			tcaac07.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			if (tcaac07["MEASURE_MODE"].ToString().Trim() == "")
			{
				sprintf(s.msg, "分摊方式无值!");
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			if (tcaac07.QueryCount("SUB_BACKLOG_CODE,MAT_CODE,DEPT_CODE") != 0)
			{
				sprintf(s.msg, "物料代码[%s]在工序[%s]收集规则中已存在!", (const char*)tcaac07["MAT_CODE"], (const char*)tcaac07["SUB_BACKLOG_CODE"]);
				throw CApplicationException(-1, s.msg, s.svc_name);
			}
			//tcaac07.Print();

			tcaac07["REC_CREATOR"] = s.userid;   //记录创建责任者
			tcaac07["REC_CREATE_TIME"] = datetime;   //记录创建时刻
			//取成本中心

			sqlstr = "SELECT COST_CENTER FROM TCAAC08"
				" WHERE SUB_BACKLOG_CODE = @tcaac07.SUB_BACKLOG_CODE"
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("tcaac07.SUB_BACKLOG_CODE", tcaac07["SUB_BACKLOG_CODE"]);
			cmd_inq.ExecuteReader();
			while (cmd_inq.Read())
			{
				tcaac07["COST_CENTER"] = cmd_inq.GetString(1);
			}
			cmd_inq.Close();

			//新增一条记录
			tcaac07.Insert();
		}
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

