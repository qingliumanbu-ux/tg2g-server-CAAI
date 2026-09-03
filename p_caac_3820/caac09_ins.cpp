/*************************************************
Copyright:   Baosight Software LTD.co Copyright (c) 2010
Author:      zhouli
Version:
Date:        2011-39-14 09:39:03
Description: 权重系数新增
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件

//外部函数声明
// service入口
BM2F_ENTERACE(caac09_ins)

int f_caac09_ins(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	/* 程序内部变量 */
	int   doFlag = 0;

	/* 业务变量 */
	CString  datetime("");

	CString  dept_code("");

	/* 实体类定义 */
	CModel tcaac09("TCAAC09");

	// 数据库SQL操作字符串
	CString  sqlstr("");

	CDbCommand cmd_inq(conn);

	try
	{
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");

		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			tcaac09.MergeFrom(bcls_rec->Tables[0].Rows[i]);

			tcaac09["REC_CREATOR"] = s.userid;   //记录创建责任者
			tcaac09["REC_CREATE_TIME"] = datetime;   //记录创建时刻

			////取产线
			//sqlstr = " SELECT CODE_LINE"
			//	" FROM tcaac14"
			//	" WHERE SUB_BACKLOG_CODE=@tcaac09.SUB_BACKLOG_CODE"
			//	;
			//cmd_inq.SetCommandText(sqlstr);
			//cmd_inq.Parameters.Set("tcaac09.SUB_BACKLOG_CODE", tcaac09["SUB_BACKLOG_CODE"].ToString());
			//cmd_inq.ExecuteReader();
			//if (cmd_inq.Read())
			//{
			//	dept_code = cmd_inq.GetString(1);
			//}
			//cmd_inq.Close();


			tcaac09["DEPT_CODE"] = dept_code;
			//新增一条记录
			tcaac09.TrimOrBlank();
			tcaac09.Insert();
		}
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.msg, (const char*)str, sizeof(s.msg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
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
